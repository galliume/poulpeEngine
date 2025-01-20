function _rotation01(animScript, deltaTime)
  while true do
    _Rotate(animScript, deltaTime, 2, 0.0, 180.0, 0.0)
    coroutine.yield()
  end
end

local animation = coroutine.create(_rotation01)

-- no move
-- function nextMove(animScript, deltaTime)
-- end

function nextRotation(animScript, deltaTime)
  if coroutine.status(animation) ~= 'dead' then
    coroutine.resume(animation, animScript, deltaTime)
  end
end
