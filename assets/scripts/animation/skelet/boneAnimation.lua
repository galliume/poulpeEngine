function _move01(animScript, deltaTime)
    while true do
      _Move(animScript, deltaTime)
      coroutine.yield()
    end
end

local animation = coroutine.create(_move01)

function nextMove(animScript, deltaTime)
  if coroutine.status(animation) ~= 'dead' then
    coroutine.resume(animation, animScript, deltaTime)
  end
end

-- no rotation
-- function nextRotation(animScript, deltaTime)
-- end